{-# LANGUAGE OverloadedStrings #-}

module Main where

import           Data.List      (foldl', intercalate)
import qualified Data.Map       as Map
import qualified Data.Text      as T
import qualified Data.Text.IO   as TIO
import qualified Data.Text.Read as TR
import           Utils          (digitCharToInt, intToList)

data Tree = EmptyTree | Node {height :: Int, childCount :: Int,children :: [Tree] } deriving (Show)

type Pincode = [Int]

type PincodeList = [Int]

type StatePincodeListDictionary = Map.Map T.Text [Int]

type StatePincodeRegexDictionary = Map.Map T.Text T.Text


initNode :: Int -> Tree
initNode h = Node {
    height = h,
    children = replicate 10 EmptyTree,
    childCount = 0
}

-- START : Build a Tree from a list of pincodes
updateChildNodes :: [Tree] -> Int -> Tree -> [Tree]
updateChildNodes childNodes index childNode =
    let (before, _:after) = splitAt index childNodes
    in before ++ [childNode] ++ after

buildTreeFromPincode :: Tree -> Pincode -> Tree
buildTreeFromPincode rootNode [] = rootNode
buildTreeFromPincode currNode@Node{height=h, children=children, childCount=childCount} (x:xs) =
    let (newChildNode,newChildCount) = case childNode of
            EmptyTree -> (buildTreeFromPincode (initNode (h+1)) xs,childCount + 1)
            childNode' -> (buildTreeFromPincode childNode' xs,childCount)
        newChildren = updateChildNodes children x newChildNode
    in currNode {children = newChildren, childCount = newChildCount}
    where
        childNode = children !! x

buildTreeFromPincodeList :: Tree -> [Pincode] -> Tree
buildTreeFromPincodeList rootNode [] = rootNode
buildTreeFromPincodeList EmptyTree (x:xs) =
    let rootNode' = buildTreeFromPincode rootNode x
    in  buildTreeFromPincodeList rootNode' xs
    where
        rootNode = initNode 0
buildTreeFromPincodeList rootNode (x:xs) =
    let rootNode' = buildTreeFromPincode rootNode x
    in buildTreeFromPincodeList rootNode' xs

-- END : Build a Tree from a list of pincodes

-- START : Build a Regex from a Tree
regexFromPincodesTree :: Tree -> String
regexFromPincodesTree EmptyTree = ""
regexFromPincodesTree Node {height=height, children=children, childCount=childrenCount}
    | childrenCount == 0 = ""
    | otherwise =
        let childRegexes = map regexFromChild [0..9]
            result = truncateRegexesList childRegexes height
        in result
    where
        regexFromChild :: Int -> String
        regexFromChild i =
            case children !! i of
                EmptyTree -> ""
                child ->
                    let childRegex = regexFromPincodesTree child
                    in if height == 0 then "^" ++ show i ++ childRegex
                        else show i ++ childRegex

truncateRegexesList :: [String] -> Int -> String
truncateRegexesList [] _ = ""
truncateRegexesList childRegexes height
    | null filteredChildRegexes = ""
    | length filteredChildRegexes == 1 = head filteredChildRegexes
    | height == 5 = "[" ++ concat filteredChildRegexes ++ "]"
    | otherwise = "(" ++ intercalate "|" filteredChildRegexes ++ ")"
    where
        filteredChildRegexes = filter (/= "") childRegexes
        concatStrings :: [String] -> String
        concatStrings = concat

pincodeListToRegex :: PincodeList -> T.Text
pincodeListToRegex pincodes =
    T.pack $ regexFromPincodesTree tree
    where
        parsedPincodes = map intToList pincodes
        tree = buildTreeFromPincodeList EmptyTree parsedPincodes

createStatePincodeRegex :: StatePincodeListDictionary -> StatePincodeRegexDictionary
createStatePincodeRegex = Map.map pincodeListToRegex
-- END : Build a Regex from a Tree

-- START : read the input/output csv from arguments
parseLines :: T.Text -> (T.Text, Int)
parseLines line =
  let [pincodeText, state] = T.splitOn "," line
      Right (pin, _) = TR.decimal pincodeText
   in (state, pin)

addPincodeToDictionary :: StatePincodeListDictionary -> (T.Text, Int) -> StatePincodeListDictionary
addPincodeToDictionary dict (state, pincode) =
  let pincodeList = Map.findWithDefault [] state dict
   in Map.insert state (pincode : pincodeList) dict

createStatePincodeListDictionary :: T.Text -> StatePincodeListDictionary
createStatePincodeListDictionary contents =
  let lines = T.lines contents
      parsedLines = map parseLines (tail lines) -- tail to remove the header
   in foldl' addPincodeToDictionary Map.empty parsedLines
-- END : read the input/output csv from arguments

printTree :: StatePincodeListDictionary -> IO ()
printTree statePincodeDic =
    let statePincodeList = Map.toList statePincodeDic
    in mapM_ buildTreeAndPrint statePincodeList
    where
        buildTreeAndPrint :: (T.Text,[Int]) -> IO()
        buildTreeAndPrint (state,pincodeList) = do
            print pincodeList'
            let tree = buildTreeFromPincodeList EmptyTree pincodeList'
            print tree
            where
                pincodeList' = map intToList pincodeList

main :: IO ()
main = do
  contents <- TIO.readFile "pincodes.csv"
  let statePincodeDic = createStatePincodeListDictionary contents
      statePincodeRegexDic = createStatePincodeRegex statePincodeDic
  print statePincodeDic
  print statePincodeRegexDic
-- END: read the input/output csv from arguments

